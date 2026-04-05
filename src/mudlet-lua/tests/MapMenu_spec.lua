describe("Tests map menu functions", function()

  setup(function()
    -- createMapper initializes mp2dMap which is required by addMapMenu/getMapMenus
    createMapper(0, 0, 400, 300)
  end)

  describe("Tests getMapMenus detailed mode", function()

    before_each(function()
      addMapMenu("_test_detail_A", "", "Detail Menu A")
      addMapMenu("_test_detail_B", "_test_detail_A", "Detail Menu B")
    end)

    after_each(function()
      removeMapMenu("_test_detail_A")
      removeMapMenu("_test_detail_B")
    end)

    it("should return an integer-indexed array usable with ipairs", function()
      local menus = getMapMenus(true)
      assert.is_table(menus)
      assert.is_not_nil(menus[1])
      assert.is_nil(menus[0])
    end)

    it("should include uniquename, name, and parent fields for each entry", function()
      local menus = getMapMenus(true)
      local found
      for _, m in ipairs(menus) do
        if m.uniquename == "_test_detail_A" then
          found = m
          break
        end
      end
      assert.is_not_nil(found, "test menu _test_detail_A not found in results")
      assert.are.equal("_test_detail_A", found.uniquename)
      assert.are.equal("Detail Menu A", found.name)
      assert.are.equal("top-level", found.parent)
    end)

    it("should report the parent uniquename for child menus", function()
      local menus = getMapMenus(true)
      local found
      for _, m in ipairs(menus) do
        if m.uniquename == "_test_detail_B" then
          found = m
          break
        end
      end
      assert.is_not_nil(found, "child menu _test_detail_B not found in results")
      assert.are.equal("_test_detail_A", found.parent)
    end)

    it("should report 'top-level' for menus with no parent", function()
      local menus = getMapMenus(true)
      for _, m in ipairs(menus) do
        if m.uniquename == "_test_detail_A" then
          assert.are.equal("top-level", m.parent)
          return
        end
      end
      assert.is_true(false, "_test_detail_A not found in results")
    end)

  end)

  describe("Tests getMapMenus order preservation", function()

    after_each(function()
      removeMapMenu("_test_order_X")
      removeMapMenu("_test_order_Y")
      removeMapMenu("_test_order_Z")
    end)

    it("should return menus in insertion order", function()
      addMapMenu("_test_order_X", "", "Order X")
      addMapMenu("_test_order_Y", "", "Order Y")
      addMapMenu("_test_order_Z", "", "Order Z")

      local menus = getMapMenus(true)
      local posX, posY, posZ
      for i, m in ipairs(menus) do
        if m.uniquename == "_test_order_X" then posX = i end
        if m.uniquename == "_test_order_Y" then posY = i end
        if m.uniquename == "_test_order_Z" then posZ = i end
      end
      assert.is_not_nil(posX, "_test_order_X not found")
      assert.is_not_nil(posY, "_test_order_Y not found")
      assert.is_not_nil(posZ, "_test_order_Z not found")
      assert.is_true(posX < posY, "X should precede Y in insertion order")
      assert.is_true(posY < posZ, "Y should precede Z in insertion order")
    end)

    it("should not produce a duplicate entry when a menu is re-added", function()
      addMapMenu("_test_order_X", "", "Order X")
      addMapMenu("_test_order_Y", "", "Order Y")
      addMapMenu("_test_order_X", "", "Order X Updated")

      local menus = getMapMenus(true)
      local count = 0
      for _, m in ipairs(menus) do
        if m.uniquename == "_test_order_X" then
          count = count + 1
        end
      end
      assert.are.equal(1, count)
    end)

  end)

  describe("Tests getMapMenus non-detailed mode", function()

    before_each(function()
      addMapMenu("_test_basic_A", "", "Basic Menu A")
    end)

    after_each(function()
      removeMapMenu("_test_basic_A")
    end)

    it("should return a table keyed by display name with parent as value", function()
      local menus = getMapMenus()
      assert.is_table(menus)
      assert.are.equal("top-level", menus["Basic Menu A"])
    end)

    it("should behave identically when called with explicit false", function()
      local menus = getMapMenus(false)
      assert.is_table(menus)
      assert.are.equal("top-level", menus["Basic Menu A"])
    end)

  end)

end)
